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

    /// Return the capacity of a FIFO or register in terms of tokens
    public static Integer getTokenCapacity(Task task) {
        assert getTaskType(task) == TaskType.MEM;
        return task.<Integer>getAttribute("smoc-token-capacity");
    }
    /// Set the capacity of a FIFO or register in terms of tokens
    public static void setTokenCapacity(Task task, int capacity) {
        assert getTaskType(task) == TaskType.MEM;
        task.setAttribute("smoc-token-capacity", capacity);
    }

    /// Return the number of initial tokens present in a FIFO
    public static Integer getInitialTokens(Task task) {
        assert getTaskType(task) == TaskType.MEM;
        return task.<Integer>getAttribute("smoc-token-initial");
    }
    /// Set the number of initial tokens present in a FIFO
    public static void setInitialTokens(Task task, int initial) {
        assert getTaskType(task) == TaskType.MEM;
        task.setAttribute("smoc-token-initial", initial);
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

    /// Return the size of a message in bytes
    public static String getMessageReadChannel(Task task) {
        assert task instanceof ICommunication;
        return task.<String>getAttribute("smoc-msg-read-channel");
    }
    /// Set the size of a message in bytes
    public static void setMessageReadChannel(Task task, String channelId) {
        assert task instanceof ICommunication;
        task.setAttribute("smoc-msg-read-channel", channelId);
    }

    /// Return channel ids (names) represented by the given MEM task
    public static Collection<String> getRepresentedChannels(Task task) {
        assert getTaskType(task) == TaskType.MEM;
        Collection<String> representedChannels = task.<Collection<String>>getAttribute("smoc-represented-channels");
        return representedChannels == null
            ? Arrays.asList(task.getId())
            : representedChannels;
    }
    /// Add a channel to be represented by the given MEM task
    public static void addRepresentedChannel(Task task, String channelId) {
        assert getTaskType(task) == TaskType.MEM;
        Collection<String> channelIds = task.<Collection<String>>getAttribute("smoc-represented-channels");
        if (channelIds == null) {
            channelIds = new ArrayList<>();
            task.setAttribute("smoc-represented-channels", channelIds);
        }
        channelIds.add(channelId);
    }
    /// Set channels represented by the given MEM task
    public static void setRepresentedChannels(Task task, Collection<String> channelIds) {
        assert getTaskType(task) == TaskType.MEM;
        task.setAttribute("smoc-represented-channels", channelIds);
    }

}
