package de.fau.scd.SNG;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Set;

import org.opt4j.core.start.Constant;

import com.google.inject.Inject;

import de.fau.scd.SNG.SNGReader.SNGFormatErrorException;
import net.sf.opendse.model.Specification;
import net.sf.opendse.optimization.SpecificationWrapper;
import net.sf.opendse.optimization.io.SpecificationTransformer;
import net.sf.opendse.optimization.io.SpecificationWrapperInstance;

public class SpecificationWrapperSNG implements SpecificationWrapper {

    final private SpecificationWrapperInstance specificationWrapperInstance;
    final private SNGReader sgxReader;

    @Inject
    public SpecificationWrapperSNG(
            @Constant(namespace = SpecificationWrapperSNG.class, value = "sgxFile") String sgxFileName)
            throws IOException, FileNotFoundException, SNGFormatErrorException {
        sgxReader = new SNGReader(sgxFileName);
        specificationWrapperInstance = new SpecificationWrapperInstance(sgxReader.getSpecification());
    }

    @Override
    public Specification getSpecification() {
        return specificationWrapperInstance.getSpecification();
    }

    @Inject(optional = true)
    public void setSpecificationTransformers(Set<SpecificationTransformer> transformers) {
        specificationWrapperInstance.setSpecificationTransformers(transformers);
    }

}
