// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2021 FAU -- Nils Wilbert <nils.wilbert@fau.de>
 *
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 *
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

package de.fau.scd.VPC.properties;

import net.sf.opendse.model.ICommunication;

import java.util.*;

//import net.sf.opendse.model.Mapping;
//import net.sf.opendse.model.Resource;
import net.sf.opendse.model.Task;

public class ApplicationPropertyService {

    private ApplicationPropertyService() {
    }

    public static Double getDeadline(Task task) {
        return task.<Double>getAttribute("smoc-actor-deadline");
    }
    public static void setDeadline(Task task, double deadline) {
        task.setAttribute("smoc-actor-deadline", deadline);
    }

    public enum TaskType { EXE, MEM, MSG };

    public static TaskType getTaskType(Task task) {
        if (task instanceof ICommunication)
            return TaskType.MSG;
        else {
            TaskType taskType = task.<TaskType>getAttribute("smoc-task-type");
            assert taskType != TaskType.MSG;
            return taskType != null ? taskType : TaskType.EXE;
        }
    }
    public static void setTaskType(Task task, TaskType taskType) {
        if (taskType == TaskType.MSG) {
            assert task instanceof ICommunication;
        } else {
            assert !(task instanceof ICommunication);
            task.setAttribute("smoc-task-type", taskType);
        }
    }

    /// Attribute name used to store the capacity of a FIFO in terms of tokens
    static public final String attrTokenCapacity = "smoc-token-capacity";

    /// Return the capacity of a FIFO or register in terms of tokens
    public static Integer getTokenCapacity(Task task) {
        assert getTaskType(task) == TaskType.MEM;
        return task.<Integer>getAttribute(attrTokenCapacity);
    }
    /// Set the capacity of a FIFO or register in terms of tokens
    public static void setTokenCapacity(Task task, int capacity) {
        assert getTaskType(task) == TaskType.MEM;
        task.setAttribute(attrTokenCapacity, capacity);
    }

    /// Attribute name used to store the initial number of tokens in a FIFO
    static public final String attrInitialToken = "smoc-token-initial";

    /// Return the number of initial tokens present in a FIFO
    public static Integer getInitialTokens(Task task) {
        assert getTaskType(task) == TaskType.MEM;
        return task.<Integer>getAttribute(attrInitialToken);
    }
    /// Set the number of initial tokens present in a FIFO
    public static void setInitialTokens(Task task, int initial) {
        assert getTaskType(task) == TaskType.MEM;
        task.setAttribute(attrInitialToken, initial);
    }

    /// Attribute name used to store the token size
    static public final String attrTokenSize = "smoc-token-size";

    /// Return the size of a token in bytes
    public static Integer getTokenSize(Task task) {
        assert getTaskType(task) == TaskType.MEM;
        return task.<Integer>getAttribute(attrTokenSize);
    }
    /// Set the size of a token in bytes
    public static void setTokenSize(Task task, int bytes) {
        assert getTaskType(task) == TaskType.MEM;
        task.setAttribute(attrTokenSize, bytes);
    }

    /// Return the size of the required storage in bytes
    public static Integer getStorageSize(Task task) {
        assert getTaskType(task) == TaskType.MEM;
        return task.<Integer>getAttribute("smoc-storage-size");
    }
    /// Set the size of the required storage in bytes
    public static void setStorageSize(Task task, int bytes) {
        assert getTaskType(task) == TaskType.MEM;
        task.setAttribute("smoc-storage-size", bytes);
    }

    /// Return the size of a message in bytes
    public static Integer getMessagePayload(Task task) {
        assert task instanceof ICommunication;
        return task.<Integer>getAttribute("smoc-msg-payload");
    }
    /// Set the size of a message in bytes
    public static void setMessagePayload(Task task, int bytes) {
        assert task instanceof ICommunication;
        task.setAttribute("smoc-msg-payload", bytes);
    }
    
    /// Return the channel ids (names) represented by a given read message
    public static Map<String, String> getRepresentedReadChannels(Task task) {
        assert task instanceof ICommunication;
        Map<String, String>  representedChannels = task.<Map<String, String> >getAttribute("smoc-represented-read-channels");       
        return representedChannels;
    }     
    /// Set channels represented by the given read message
    public static void setRepresentedReadChannels(Task task, Map<String, String> channelIds) {
        assert task instanceof ICommunication;
        task.setAttribute("smoc-represented-read-channels", channelIds);
    }
    
    /// Return the channel ids (names) represented by a given write message
    public static Map<String, Set<String>>getRepresentedWriteChannels(Task task) {
        assert task instanceof ICommunication;
        Map<String, Set<String>>  representedChannels = task.<Map<String, Set<String>> >getAttribute("smoc-represented-write-channels");
        return representedChannels;
    }     
    /// Set channels represented by the given write message
    public static void setRepresentedWriteChannels(Task task, Map<String, Set<String>> channelIds) {
        assert task instanceof ICommunication;
        task.setAttribute("smoc-represented-write-channels", channelIds);
    }
    
    public static int getChannelSize(String channelId) {
        assert channelSizes.containsKey(channelId);
        return channelSizes.get(channelId);
    }
    
    public static void setChannelSize(String channelId, int tokenSize) {
        channelSizes.put(channelId, tokenSize);
    }
    
    private static Map<String, Integer> channelSizes = new HashMap<>();

}
